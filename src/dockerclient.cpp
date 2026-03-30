/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dockerclient.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>

#include <KLocalizedString>

DockerClient::DockerClient(QObject *parent)
    : QObject(parent)
{
}

DockerClient::~DockerClient()
{
}

void DockerClient::checkAvailability()
{
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    proc->setArguments({QStringLiteral("info"),
                        QStringLiteral("--format"),
                        QStringLiteral("{{json .}}")});

    connect(proc, &QProcess::finished, this,
            [this, proc](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                if (exitCode == 0) {
                    Q_EMIT availabilityChanged(DockerAvailability::Available, QString());
                } else {
                    const DockerAvailability av = classifyError(stdErr);
                    Q_EMIT availabilityChanged(av, stdErr.trimmed());
                }
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError error) {
                proc->deleteLater();
                if (error == QProcess::FailedToStart) {
                    Q_EMIT availabilityChanged(DockerAvailability::NotFound,
                                               i18n("'docker' executable not found in PATH"));
                } else {
                    Q_EMIT availabilityChanged(DockerAvailability::Error,
                                               i18n("Failed to start docker process"));
                }
            });

    proc->start();
}

void DockerClient::listContainers(bool showAll)
{
    auto *proc = new QProcess(this);
    proc->setProgram(QStringLiteral("docker"));
    QStringList args = {QStringLiteral("ps"),
                        QStringLiteral("--format"),
                        QStringLiteral("{{json .}}")};
    if (showAll) {
        args.append(QStringLiteral("-a"));
    }
    proc->setArguments(args);

    connect(proc, &QProcess::finished, this,
            [this, proc](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
                const QByteArray stdOut = proc->readAllStandardOutput();
                const QString stdErr = QString::fromUtf8(proc->readAllStandardError());
                proc->deleteLater();

                if (exitCode != 0) {
                    Q_EMIT availabilityChanged(DockerAvailability::Error, stdErr.trimmed());
                    return;
                }

                const QList<ContainerInfo> containers = parseContainerOutput(stdOut);
                Q_EMIT containersUpdated(containers);
            });

    connect(proc, &QProcess::errorOccurred, this,
            [this, proc](QProcess::ProcessError /*error*/) {
                proc->deleteLater();
                Q_EMIT availabilityChanged(DockerAvailability::Error,
                                           i18n("Failed to run 'docker ps'"));
            });

    proc->start();
}

DockerAvailability DockerClient::classifyError(const QString &stdErr) const
{
    if (stdErr.contains(QLatin1String("permission denied"), Qt::CaseInsensitive)) {
        return DockerAvailability::PermissionDenied;
    }
    if (stdErr.contains(QLatin1String("Cannot connect"), Qt::CaseInsensitive)
        || stdErr.contains(QLatin1String("Is the docker daemon running"), Qt::CaseInsensitive)) {
        return DockerAvailability::DaemonDown;
    }
    return DockerAvailability::Error;
}

QList<ContainerInfo> DockerClient::parseContainerOutput(const QByteArray &data) const
{
    QList<ContainerInfo> result;
    const QList<QByteArray> lines = data.split('\n');

    for (const QByteArray &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        ContainerInfo info;
        info.id = obj.value(QLatin1String("ID")).toString();
        info.name = obj.value(QLatin1String("Names")).toString();
        info.image = obj.value(QLatin1String("Image")).toString();
        info.status = obj.value(QLatin1String("Status")).toString();
        info.ports = obj.value(QLatin1String("Ports")).toString();
        result.append(info);
    }

    return result;
}

#include "moc_dockerclient.cpp"
